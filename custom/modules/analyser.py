
from dataclasses import dataclass

from modules.constants import RESOURCE_PATH


@dataclass
class DataRow:
    user: int
    checkin_time:str
    latitude:str
    longitude:str
    location_id:str


def convert_csv_to_dict(path, limit):
    """
    Convert a csv to a dictionary like data structure
    """
    file = open(path, 'r')
    res = []
    cpt = 1
    for line in file.readlines():
        if cpt <= limit:
            row = DataRow(
                user=line.split("\t")[0],
                checkin_time=line.split("\t")[1],
                latitude=line.split("\t")[2],
                longitude=line.split("\t")[3],
                location_id=line.split("\t")[4],
            )
            res.append(row)
            cpt = cpt + 1
        else:
            break
    return res
    


def display_list_10_first(l):
    cpt = 1
    for item in l:
        if cpt <= 10:
            print(item)
            cpt = cpt + 1
        else:
            break


def test():
    res = convert_csv_to_dict(RESOURCE_PATH, 10)
    display_list_10_first(res)


